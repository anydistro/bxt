import { Button, Drawer, DrawerProps, Menu } from "react-daisyui";
import CommitCard from "./CommitCard";
import { SectionUtils } from "../utils/SectionUtils";

type CommitDrawerProps = DrawerProps & {
    isOpen: boolean;
    commits: Commits;
    onPush: (commits: Commits) => void;
    onCardActivate: (section: Section, commit: Commit) => void;
    onCardDelete: (section: Section) => void;
};

export default function CommitModal(props: CommitDrawerProps) {
    return (
        <Drawer
            {...props}
            open={props.commits.size > 0 && props.isOpen}
            contentClassName="fm-content h-full"
            className="h-full"
            side={
                <div>
                    <label
                        htmlFor="my-drawer-2"
                        className="drawer-overlay"
                    ></label>
                    <Menu className="h-screen p-4 w-100 space-y-4 bg-accent">
                        <li className="text-3xl font-bold text-white">
                            Pending commits
                        </li>
                        {Array.from(props.commits).map(([section, commit]) => {
                            return (
                                <Menu.Item>
                                    <CommitCard
                                        section={SectionUtils.fromString(
                                            section
                                        )}
                                        onActivate={props.onCardActivate}
                                        commit={commit}
                                        onDeleteRequested={props.onCardDelete}
                                    />
                                </Menu.Item>
                            );
                        })}
                        <div className="grow"></div>
                        <Button
                            color="ghost"
                            className="text-white"
                            onClick={(e) => props.onPush(props.commits)}
                        >
                            Push commits
                        </Button>
                    </Menu>
                </div>
            }
            end={true}
        />
    );
}
